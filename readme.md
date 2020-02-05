VST Interceptor
===============

This is a very simple VST plugin wrapper that logs the VST messages sent from the plugin to the host. This maybe useful in debugging the interaction between VST host and plugin. It is based on ValdemarOrn's VST plugin wrapper that was designed for changing VST plugin IDs.

Usage is simple: Just build and rename the file in the following way:

    PluginName.Intercept.1234567890.dll
    
Launch the debugger, and VST messages will be printed to the Visual Studio Debug Window.

Where PluginName is the name of a matching "PluginName.dll" VST Plugin that you want to wrap. The 10-digit number is your new VstId so the wrapped plugin doesn't conflict with the wrapped version. **Note: It must contain exactly 10 digits, pad the number with leading zeros if necessary!**

ValdemarOrn's notes:
--------------------
The code is also useful in case you want to intercept more operations in the a plugin. Feel free to use as you see fit.

The code is public domain, do with it what you please, but don't holde me responsible if you make a mess!




