# Browser

_(The name is going to change one day, I promise.)_

**Browser** is a web browser written from scratch in C++.

## Building

The build process uses Meson and Ninja.

Configure the build directory like so:

```console
$ meson setup builddir/
```

Meson will look for the cURL library and the Qt framework and enable their respective shells.

Once it's configured, run:

```console
$ ninja
```

Now you're building Browser.

## cURL Shell

The cURL shell (executable `shells/curl/browser_curl`) is a test shell using `libcurl`.

To use it, run it with the first argument being the website that you want to visit. Once it's done parsing, it will output a `test.ppm` image with the rendered content.

## Qt shell

The Qt shell (executable `shells/qt/browser_qt`) is a shell which uses Qt for requests, URL control and displaying rendered content.

To use it, just run it, enter your URL and press enter, and wait a second for it to load all data.
