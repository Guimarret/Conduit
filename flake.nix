{
  description = "Flake to run conduit anywhere";

  inputs = {
     nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
     flake-utils.url = "github:numtide/flake-utils";
   };

   outputs = { self, nixpkgs, flake-utils }:
     flake-utils.lib.eachDefaultSystem (system:
       let
         pkgs = nixpkgs.legacyPackages.${system};
       in {
         packages.default = pkgs.stdenv.mkDerivation {
           name = "conduit";
           src = ./.;

           nativeBuildInputs = [ pkgs.makeWrapper ];
           buildInputs = [ pkgs.sqlite ];

           buildPhase = ''
             make CC=${pkgs.stdenv.cc}/bin/cc \
                  CFLAGS="-Wall -Werror" \
                  LDFLAGS="-lsqlite3"
           '';

           installPhase = ''
             mkdir -p $out/bin
             cp output $out/bin/
           '';

           meta = with pkgs.lib; {
             license = licenses.mit;
             platforms = platforms.all;
           };
         };

         devShells.default = pkgs.mkShell {
           buildInputs = [
             pkgs.sqlite
             pkgs.gnumake
             pkgs.stdenv.cc
           ];

           shellHook = ''
             echo "SQLite3 version: ${pkgs.sqlite.version}"
           '';
         };
       });
 }
