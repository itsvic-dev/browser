with import <nixpkgs> {};
stdenv.mkDerivation {
    name = "dev-environment"; # Maybe put a more meaningful name here
    buildInputs = [
        pkg-config
        curl
        freetype
        fontconfig
        lcov
        qt6.qtbase
        qt6.qtwayland
        qt6.qmake
    ];
}
