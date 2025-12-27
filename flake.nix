{
  description = "c development environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      inherit (nixpkgs.lib)
        genAttrs
        platforms
        fileset
        ;
      systems = platforms.unix;
      eachSystem =
        f:
        genAttrs systems (
          system:
          f (
            import nixpkgs {
              inherit system;
              config = { };
              overlays = [ ];
            }
          )
        );
      fs = fileset;
      root = ./.;
      src = fs.toSource {
        inherit root;
        fileset = fs.intersection (fs.gitTracked root) (
          fs.fileFilter (f: (f.hasExt "c") || (f.hasExt "h")) ./src
        );
      };
      pname = "yamdw";
    in
    {
      asdf = src;
      devShells = eachSystem (pkgs: {
        default = pkgs.mkShellNoCC {
          packages = with pkgs; [ tinycc ];
        };
      });

      packages = eachSystem (
        pkgs:
        let
          inherit (pkgs) coreutils dash tinycc;
          inherit (pkgs.lib) getExe concatMapStringsSep;
        in
        {
          default = derivation {
            name = "${pname}-0.0.1";
            inherit (pkgs) system;
            inherit src;
            builder = getExe dash;
            args = [
              "-c"
              # TODO: static
              ''
                PATH="$PATH:${coreutils}/bin"
                mkdir -p "$out/bin" &&
                ${getExe tinycc} "$src/src/"*.h "$src/src/"*.c \
                  "$src/src/md4c/"*.h "$src/src/md4c/"*.c \
                  -o "${pname}" -std=c99 -fsanitize=address -O3 &&
                  # -Wall -Werror -Wextra
                mv "${pname}" "$out/bin"
              ''
            ];
          };
        }
      );

      apps = eachSystem (pkgs: {
        default = {
          type = "app";
          program = "${self.packages.${pkgs.system}.default}/bin/${pname}";
        };
      });
    };
}
