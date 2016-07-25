# HomeApp
Using Speech Recognition software Pocketsphinx to connect to PSAP voiceapp
This is modified from pocketsphinxcontinuous in order to connect with
the PSAP avatar on a local host in order to facilitate voice interaction with
avatar which interacts with a smart home.
Requires latest pocketsphinx and sphinxbase to be installed
The jsgf grammar is attatched to improve accuracy.
----------------------------------------------------------------------------------------
In order to run the c version, you need:
*Pocketsphinx
*Sphinxbase

-----------------------------------------------------------------------------------------
In order to run python version, you need:
*Pocketsphonx
*Sphinxbase
*Gstreamer 1.0
*pygtk/gtk+
*Python
(webbrowser should be included in python)

>> Note: the PSAP Avatar is browser dependent. It will output audio regardless of browser, 
but for the mouth to move, it's recommended that you use Mozilla Firefox.
