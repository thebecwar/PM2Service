# PM2Service
PM2 Service Wrapper for Windows

This is a work in progress. It's probably not ready to use, unless you write the rest of the code to make it work, but if you do, please submit a PR.

Contributions are welcome, under the principle of "seriously be nice"

# Installation

Until there's an actual release, download the [latest nightly](https://dev.azure.com/thebecwar/pm2service/_build?definitionId=2). Once you have release:

1. Create a directory for the app bundle at `C:\Program Files\PM2 Service` and extract the contents of the zip there. There will be two files in the zip, an exe (the service executable) and a dll (the error/message eventlog dll).
2. Create a local user that the service will run under. You don't need to configure the user for service login, but there needs to be a local account for the service to execute.
3. Open an administrative command prompt and execute the following commands:
   ```
   cd "C:\Program Files\PM2 Service"
   pm2service install USERNAME PASSWORD
   pm2service setpm2version 3.5.1
   ```
4. Start the service from the services utility or by executing `pm2service start`

Once the service is running you can interact with PM2 from within any administrative command prompt by setting `PM2_HOME` to `%PROGRAMDATA\PM2Service`. The service will run `pm2 resurrect` when it starts/restarts, so pm2 save will function as expected.

_An installer is planned for the future to automate this, but it's not ready for prime-time yet._

