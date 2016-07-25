/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Copyright (c) 1999-2010 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
 //***Modified for Smart-Environment***//


#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <sys/select.h>
#endif

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include "pocketsphinx.h"

 static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    {"-argfile",
     ARG_STRING,
     NULL,
     "Argument file giving extra arguments."},
    {"-adcdev",
     ARG_STRING,
     NULL,
     "Name of audio device to use for input."},
    {"-infile",
     ARG_STRING,
     NULL,
     "Audio file to transcribe."},
	{"-inmic",
     ARG_BOOLEAN,
     "no",
     "Transcribe audio from microphone."},
    {"-time",
     ARG_BOOLEAN,
     "no",
     "Print word times in file transcription."},
    CMDLN_EMPTY_OPTION
 };

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;

static void
print_word_times()
{
    int frame_rate = cmd_ln_int32_r(config, "-frate");
    ps_seg_t *iter = ps_seg_iter(ps);
    while (iter != NULL) {
        int32 sf, ef, pprob;
        float conf;

        ps_seg_frames(iter, &sf, &ef);
        pprob = ps_seg_prob(iter, NULL, NULL, NULL);
        conf = logmath_exp(ps_get_logmath(ps), pprob);
        printf("%s %.3f %.3f %f\n", ps_seg_word(iter), ((float)sf / frame_rate),
               ((float) ef / frame_rate), conf);
        iter = ps_seg_next(iter);
    }
}

static int
check_wav_header(char *header, int expected_sr)
{
    int sr;

    if (header[34] != 0x10) {
        E_ERROR("Input audio file has [%d] bits per sample instead of 16\n", header[34]);
        return 0;
    }
    if (header[20] != 0x1) {
        E_ERROR("Input audio file has compression [%d] and not required PCM\n", header[20]);
        return 0;
    }
    if (header[22] != 0x1) {
        E_ERROR("Input audio file has [%d] channels, expected single channel mono\n", header[22]);
        return 0;
    }
    sr = ((header[24] & 0xFF) | ((header[25] & 0xFF) << 8) | ((header[26] & 0xFF) << 16) | ((header[27] & 0xFF) << 24));
    if (sr != expected_sr) {
        E_ERROR("Input audio file has sample rate [%d], but decoder expects [%d]\n", sr, expected_sr);
        return 0;
    }
    return 1;
}

static void
parse_sentence(char const *command)
{
	//list of: Appliances,actions,aliases,percents
	char *appliances[] = {"light","door","air","heat","fan","dimmer"};
	char *actions[] = {"open","off","up","down","unlock"," lock", " on","close"};
	char *alias[] = {"front" ,"back" ,"patio" ,"side" ,"bedroom" ,"bathroom" ,"kitchen" ,"upstairs" ,"downstairs"};
	char *percent[] = {"zero","ten","twenty","thirty","forty","fifty", "sixty", "seventy",
	"eighty","ninety","one hundred","a hundred","twenty five","seventy five"};
	
	//For-loop iterator lengths
	int len1 = 6;
	int len2 = 8;
	int len3 = 9;
	int len4 = 14;
	
	//variables to hold the found values in the phrases recognized
	char app[1024] = "unassigned";
	char act[1024] = "unassigned";
	char ali[1024] = "unassigned";
	char num[1024] = "unassigned";
	
	//variable to hold the finished url that will be called
	char urlstr[1024] = "open http://10.8.0.60:8080/PSAPWebApp/smarthome/";
	
	//for loop to find key words in sentences
	//for-loop1: Finds the appliances
	for(int i = 0; i < len1; i++) {
        if(strstr(command, appliances[i]) != NULL){
        	printf("Recognized: (%d) %s \n", i, appliances[i]);
        	strcpy(app, appliances[i]);
        }
    }
    
    //for-loop2: finds the actions
    for(int x = 0; x < len2; x++) {
        if(strstr(command, actions[x]) != NULL){
        	printf("Action: (%d) %s \n", x, actions[x]);
        	strcpy(act, actions[x]);
        	break;
        }
        else{
    		if(x == len2-1 && strstr(command, actions[x]) == NULL){
    			printf("Action: involves setting object to degrees or percentage, is a status.\n"); 
    		}
    	}
    }
    //for-loop3: finds the alias used
    for(int y = 0; y < len3; y++) {
        if(strstr(command, alias[y]) != NULL){
        	printf("Alias: (%d) %s \n", y, alias[y]);
        	strcpy(ali, alias[y]);
        	break;
        }
        else{
    		if(y == len3-1 && strstr(command, alias[y]) == NULL){
    			printf("Alias: No alias used.\n"); 
    		}
    	}
    }
    //for-loop4: finds out wether or not a number/percentage is being used
    for(int z = 0; z < len4; z++) {
        if(strstr(command, percent[z]) != NULL){
        	printf("Percent: (%d) %s \n", z, percent[z]);
        	strcpy(num, percent[z]);
        	break;
        }
        else{
    		if(z == len4-1 && strstr(command, percent[z]) == NULL){
    			printf("Percent: No percentage used.\n"); 
    		}
    	}
    }
    //Prints the found variables for debugging purposes
    printf("---app:%s---act:%s---ali:%s---num:%s---\n",app,act,ali,num);
    
    //Generating the url:
    //generates url for light
    if(strcmp(app, "light") == 0){
    	if(strcmp(ali,"unassigned") != 0 && strcmp(act,"unassigned") != 0){
    		strcat(urlstr,"switchedlight/kiraAvatar/");
    		strcat(urlstr,ali);
    		strcat(urlstr,"/");
    		if(strcmp(act, " on") == 0){
    		 	strcpy(act, "on");
    			strcat(urlstr,act);
    		}
      		else{
     			strcat(urlstr,act);
    		}
    	}
    	else if(strcmp(ali,"unassigned") != 0){
    		strcat(urlstr,"switchedlight/kiraAvatar/");
    		strcat(urlstr,ali);
    		strcat(urlstr,"/");
    	}
    	else{
    		strcat(urlstr,"switchedlight/kiraAvatar/");
			if(strcmp(act, " on") == 0){
    		 	strcpy(act, "on");
    			strcat(urlstr,act);
    		}
    		else{
    			strcat(urlstr,act);
    		}
    	}
    	
    } 
    
    //generates url for dimmer
    if(strcmp(app, "dimmer") == 0){
    	if(strcmp(ali,"unassigned") != 0 && strcmp(act,"unassigned") != 0){
    		strcat(urlstr,"dimmedlight/kiraAvatar/");
    		strcat(urlstr,ali);
    		strcat(urlstr,"/");
    		if(strcmp(act, " on") == 0){
    		 	strcpy(act, "on");
    			strcat(urlstr,act);
    		}
    		else{
    			strcat(urlstr,act);
    		}
    		if(strcmp(num,"unassigned") != 0){
    			strcat(urlstr,num);
    		}
    	}
    	else if(strcmp(ali,"unassigned") != 0){
    		strcat(urlstr,"dimmedlight/kiraAvatar/");
    		strcat(urlstr,ali);
    		strcat(urlstr,"/");
    		if(strcmp(num,"unassigned") != 0){
    			strcat(urlstr,num);
    		}
    	}
    	else{
    		strcat(urlstr,"dimmedlight/kiraAvatar/");
			if(strcmp(act, " on") == 0){
    		 	strcpy(act, "on");
    			strcat(urlstr,act);
    		}
    		else if(strcmp(num,"unassigned") != 0){
    			strcat(urlstr,num);
    		}
    		else{
    			strcat(urlstr,act);
    		}
    	}
    }
   //Prints the finished url for debugging purposes
   printf(">> %s \n \n", urlstr);
   
   //Calls the finished url
   system(urlstr);
	
	
}

/*
 * Main utterance processing loop:
 *     for (;;) {
 *        start utterance and wait for speech to process
 *        decoding till end-of-utterance silence will be detected
 *        print utterance result;
 *     }
 */
static void
recognize_from_microphone()
{
    ad_rec_t *audio;
    int16 audioBuff[2048];
    uint8 utt_started, in_speech;
    int32 k;
    char const *hyp;

	//Opens audio device;
    if ((audio = ad_open_dev(cmd_ln_str_r(config, "-adcdev"), (int) cmd_ln_float32_r(config,"-samprate"))) == NULL)
        E_FATAL("Failed to open audio device\n");
    //Starts recording
    if (ad_start_rec(audio) < 0)
        E_FATAL("Failed to start recording\n");
    
    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");
        utt_started = FALSE;
        E_INFO("***....Ready....***\n");

    for (;;) {
        if ((k = ad_read(audio, audioBuff, 2048)) < 0)
            E_FATAL("Failed to read audio\n");
        ps_process_raw(ps, audioBuff, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);//returns true if words are spoken and false otherwise
        if (in_speech && !utt_started) {
            utt_started = TRUE;
            E_INFO("***...Listening...***\n");
        }
        if (!in_speech && utt_started) {
            /* speech -> silence transition, time to start new utterance  */
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );
            if (hyp != NULL) {
                printf("\n ******** YOU SAID: \" %s \" *********\n \n \n", hyp);
                fflush(stdout);
                parse_sentence(hyp);
            	exit(1);// Exits the code after the sentence has been parsed
            }

            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
                exit(1);// Exits the code if could not recognize anything
        }
        
    }
    exit(1);
    ad_close(audio);
    
    
}

int
main(int argc, char *argv[])
{
    char const *cfg;

    config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);




    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return 1;
    }

    recognize_from_microphone();

    ps_free(ps);
    cmd_ln_free_r(config);

    return 0;
}
