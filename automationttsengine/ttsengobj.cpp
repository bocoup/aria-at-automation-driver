// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright � Microsoft Corporation. All rights reserved

/*******************************************************************************
* TtsEngObj.cpp *
*---------------*
*   Description:
*       This module is the main implementation file for the CTTSEngObj class.
*
*******************************************************************************/

//--- Additional includes
#include "stdafx.h"
#include "TtsEngObj.h"
#include <stdio.h>
#include <iostream>
#include <windows.h>

//--- Local

/*****************************************************************************
* CTTSEngObj::FinalConstruct *
*----------------------------*
*   Description:
*       Constructor
*****************************************************************************/
HRESULT CTTSEngObj::FinalConstruct()
{

    HRESULT hr = S_OK;

    //--- Init vars
    m_hVoiceData = NULL;
    m_pVoiceData = NULL;
    m_pWordList  = NULL;
    m_ulNumWords = 0;

    hr = m_cpVoice.CoCreateInstance(CLSID_SpVoice);
    // The next line is the wrong direction:
    //m_cpVoice->SetNotifyCallbackFunction(StaticNotifyCallback, 0, reinterpret_cast<LPARAM>(this));

    return hr;
} /* CTTSEngObj::FinalConstruct */

/*****************************************************************************
* CTTSEngObj::FinalRelease *
*--------------------------*
*   Description:
*       destructor
*****************************************************************************/
void CTTSEngObj::FinalRelease()
{


    delete m_pWordList;

    if( m_pVoiceData )
    {
        ::UnmapViewOfFile( (void*)m_pVoiceData );
    }

    if( m_hVoiceData )
    {
        ::CloseHandle( m_hVoiceData );
    }

} /* CTTSEngObj::FinalRelease */

//
//=== ISpObjectWithToken Implementation ======================================
//

/*****************************************************************************
* CTTSEngObj::SetObjectToken *
*----------------------------*
*   Description:
*       This function performs the majority of the initialization of the voice.
*   Once the object token has been provided, the filenames are read from the
*   token key and the files are mapped.
*****************************************************************************/
STDMETHODIMP CTTSEngObj::SetObjectToken(ISpObjectToken * pToken)
{
    return SpGenericSetObjectToken(pToken, m_cpToken);
} /* CTTSEngObj::SetObjectToken */

std::string to_utf8(const std::wstring& s, ULONG length)
{
    std::string utf8;
    int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), length, NULL, 0, NULL, NULL);
    if (len > 0)
    {
        utf8.resize(len);
        WideCharToMultiByte(CP_UTF8, 0, s.c_str(), length, &utf8[0], len, NULL, NULL);
    }
    return utf8;
}

int emit(LPCTSTR words, ULONG len) {
    HANDLE pipe = CreateFile(
        L"\\\\.\\pipe\\my_pipe",
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (pipe == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to connect to pipe.");
        return 0;
    }

    DWORD numBytesWritten = 0;
    std::string stringBuffer = to_utf8(words, len);
    BOOL result = WriteFile(
        pipe, // handle to our outbound pipe
        stringBuffer.c_str(), // data to send
        stringBuffer.size(),
        &numBytesWritten, // will store actual amount of data sent
        NULL // not using overlapped IO
    );

    // Close the pipe (automatically disconnects client too)
    CloseHandle(pipe);

    if (!result) {
        fprintf(stderr, "Failed to send data.");
        return 1;
    }

    return 0;
}

//
//=== ISpTTSEngine Implementation ============================================
//

/*****************************************************************************
* CTTSEngObj::Speak *
*-------------------*
*   Description:
*       This is the primary method that SAPI calls to render text.
*-----------------------------------------------------------------------------
*   Input Parameters
*
*   pUser
*       Pointer to the current user profile object. This object contains
*       information like what languages are being used and this object
*       also gives access to resources like the SAPI master lexicon object.
*
*   dwSpeakFlags
*       This is a set of flags used to control the behavior of the
*       SAPI voice object and the associated engine.
*
*   VoiceFmtIndex
*       Zero based index specifying the output format that should
*       be used during rendering.
*
*   pTextFragList
*       A linked list of text fragments to be rendered. There is
*       one fragement per XML state change. If the input text does
*       not contain any XML markup, there will only be a single fragment.
*
*   pOutputSite
*       The interface back to SAPI where all output audio samples and events are written.
*
*   Return Values
*       S_OK - This should be returned after successful rendering or if
*              rendering was interrupted because *pfContinue changed to FALSE.
*       E_INVALIDARG 
*       E_OUTOFMEMORY
*
*****************************************************************************/
STDMETHODIMP CTTSEngObj::Speak( DWORD dwSpeakFlags,
                                REFGUID rguidFormatId,
                                const WAVEFORMATEX * pWaveFormatEx,
                                const SPVTEXTFRAG* pTextFragList,
                                ISpTTSEngineSite* pOutputSite )
{
    //--- Check args
    if(SP_IS_BAD_INTERFACE_PTR(pOutputSite) || SP_IS_BAD_READ_PTR(pTextFragList)) {
        return E_INVALIDARG;
    }
    HRESULT hr = S_OK;

    /**/
    ULONGLONG event_mask =
        SPFEI(SPEI_START_INPUT_STREAM) |
        SPFEI(SPEI_TTS_BOOKMARK) |
        SPFEI(SPEI_WORD_BOUNDARY) |
        SPFEI(SPEI_SENTENCE_BOUNDARY) |
        SPFEI(SPEI_END_INPUT_STREAM);
    m_cpVoice->SetInterest(event_mask, event_mask);
    //*/
    m_cpVoice->SetPriority(SPVPRI_ALERT);

    for (const SPVTEXTFRAG* textFrag = pTextFragList; textFrag != NULL; textFrag = textFrag->pNext) {
        if (textFrag->State.eAction == SPVA_Bookmark) {
            continue;
        }

        const std::wstring& text = textFrag->pTextStart;
        const HANDLE handle = m_cpVoice->SpeakCompleteEvent();
        m_cpVoice->Speak(text.substr(0, textFrag->ulTextLen).c_str(), dwSpeakFlags | SPF_ASYNC | SPF_PURGEBEFORESPEAK, 0);
        bool isWaiting = true;
        int timeoutCount = 0;
        while (isWaiting) {
            DWORD dwWaitId = ::MsgWaitForMultipleObjectsEx(1, &handle, 500, 0, 0);
            /**
             * This section should invoke `pOutputSite->GetActions()` to determine if any one
             * of the `SPVESACTIONS` has occurred and if so, carry out the action.
             */
            switch (dwWaitId) {
                case WAIT_OBJECT_0:
                    emit(L"Done 1", 6);
                    isWaiting = false;
                    break;
                case WAIT_OBJECT_0 + 1:
                    emit(L"Done 2", 6);
                    WaitMessage();
                    //isWaiting = false;
                    break;
                case WAIT_TIMEOUT:
                    emit(L"WAIT_TIMEOUT", 12);
                    timeoutCount += 1;
                    isWaiting = timeoutCount < 10;
                    break;
                default:
                    emit(L"default", 7);
            }
        }

        if (emit(textFrag->pTextStart, textFrag->ulTextLen) != 0) {
            break;
        }
    }

    return S_OK;
} /* CTTSEngObj::Speak */


void CTTSEngObj::StaticNotifyCallback(WPARAM wParam, LPARAM lParam) {
    CTTSEngObj* pThis = reinterpret_cast<CTTSEngObj*>(lParam);
    SPVOICESTATUS* status;
    pThis->m_cpVoice->GetStatus(status, NULL);
    emit(L"Hello", 5);
}


/*****************************************************************************
* CTTSEngObj::GetVoiceFormat *
*----------------------------*
*   Description:
*       This method returns the output data format associated with the
*   specified format Index. Formats are in order of quality with the best
*   starting at 0.
*****************************************************************************/
STDMETHODIMP CTTSEngObj::GetOutputFormat( const GUID * pTargetFormatId, const WAVEFORMATEX * pTargetWaveFormatEx,
                                          GUID * pDesiredFormatId, WAVEFORMATEX ** ppCoMemDesiredWaveFormatEx )
{
    return SpConvertStreamFormatEnum(SPSF_11kHz16BitMono, pDesiredFormatId, ppCoMemDesiredWaveFormatEx);
} /* CTTSEngObj::GetVoiceFormat */


