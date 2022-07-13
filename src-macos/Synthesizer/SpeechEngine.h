/*
	File:		SpeechEngine.h

	Contains:	Definition of the SPI between the Speech Synthesis API and a speech engine that
			implements the actual synthesis technology.  Each voice is matched to its appropriate
			speech engine via a type code stored in the voice.

			This documentation requires an understanding of the Speech Synthesis Manager
	Version:	1.0

	Copyright:	� 2000-2007 by Apple Inc., all rights reserved.

*/

/* Token to identify your private per-channel data */
typedef long SpeechChannelIdentifier;


/* API: These functions must be defined and exported with these names and extern "C" linkage. All of them
   return an OSStatus result.
*/


#ifdef __cplusplus
extern "C" {
#endif

long	SEOpenSpeechChannel	( SpeechChannelIdentifier* ssr );

long 	SEUseVoice 			( SpeechChannelIdentifier ssr, VoiceSpec* voice, CFBundleRef inVoiceSpecBundle );

long	SECloseSpeechChannel( SpeechChannelIdentifier ssr );

long 	SESpeakCFString			( SpeechChannelIdentifier ssr, CFStringRef text, CFDictionaryRef options);
long 	SECopySpeechProperty	( SpeechChannelIdentifier ssr, CFStringRef property, CFTypeRef * object );
long 	SESetSpeechProperty		( SpeechChannelIdentifier ssr, CFStringRef property, CFTypeRef object);
long 	SEUseSpeechDictionary 	( SpeechChannelIdentifier ssr, CFDictionaryRef speechDictionary );
long 	SECopyPhonemesFromText 	( SpeechChannelIdentifier ssr, CFStringRef text, CFStringRef * phonemes);

#ifdef __cplusplus
}
#endif

