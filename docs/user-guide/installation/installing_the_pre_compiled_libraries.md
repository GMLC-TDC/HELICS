# Installing the Pre-Compiled Libraries

Download pre-compiled libraries from the [releases page](https://github.com/GMLC-TDC/HELICS/releases/latest) and add them to your path.
Windows users should install the latest version of the [Visual C++ Redistributable](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads).
The installers come with bindings for Java extensions precompiled as part of the installation.
All you need to do is add the relevant folders to your User's PATH variables.

On Windows, you can visit `Control Panel -> System -> Advanced System Settings -> Environment Variables` and edit your user environment variables to add the necessary Path, JAVAPATH environment variables to the corresponding HELICS installed locations.

On MacOS or Linux, you can edit your `~/.bashrc` to add the necessary PATH, JAVAPATH environment variables to the corresponding HELICS installed locations.

Be sure to restart your CMD prompt on Windows or Terminal on your MacOS/Linux to ensure the new environment variables are in effect.
