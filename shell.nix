with import <nixpkgs> {};
stdenv.mkDerivation {
    name = "dev-environment"; # Maybe put a more meaningful name here
    buildInputs = [
        pkg-config
        curl
        lcov
    ];
}
