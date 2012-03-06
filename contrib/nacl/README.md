# Load81 Port to Google Native Client

## Requirements

* Google Chrome 15 or higher.
* Google App Engine Python SDK.
* Native Client SDK.
* NaclPorts with SDL and SDL_gfx built for i686 and x86\_64.
* NACL\_SDK\_ROOT environment variable set.

## Building

Ensure the above requirements are met, then run contrib/nacl/build.sh.

## Testing

Run dev_appserver.py contrib/nacl to start an HTTP server on port 8080. Follow
the instructions in the [Native Client documentation][1] to enable Native
Client in Chrome 15+. Now, open http://localhost:8080/ in Chrome and Load81
should start.

[1]: https://developers.google.com/native-client/pepper16/devguide/devcycle/running#Local

## Publishing

You will need to create a Google App Engine application first. Then:

    appcfg.py -A $YOUR_APP_NAME update contrib/nacl

Now you need to create a Chrome application. Follow the instructions at the
[Chrome Developer's Guide][2], using contrib/nacl/chrome-app/manifest.json as
a template.

[2]: http://code.google.com/chrome/apps/docs/developers_guide.html

## TODO

* Support loading and saving local files.
