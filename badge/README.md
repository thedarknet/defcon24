Install Dev Env:
* go here:  http://gnuarmeclipse.github.io/ for background and step by step

Basic steps:

** Install eclipse Luna CDT (c++ env)

** add a new market place:  


name: GNU ARM Eclipse Plug-ins
URL: http://gnuarmeclipse.sourceforge.net/updates

You will see the ecplise arm plugin select it and it will install for you.

If you get complaints about missing software I found these links helpful.

** apt-get install gcc-arm-none-eabi (or something close to that)

** apt-get install openocd

*** Or download from 

*** to configure openocd in eclipse look here: http://gnuarmeclipse.github.io/debug/openocd/

*** to download new version of openocd:  http://gnuarmeclipse.github.io/openocd/download/


then close this repo and then import this directory into you're workspace (remeber to uncheck the option to copy all files to your workspace).

