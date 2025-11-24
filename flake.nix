{
  description = "c development environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = nixpkgs.lib.platforms.unix;
      eachSystem =
        f:
        nixpkgs.lib.genAttrs systems (
          system:
          f (
            import nixpkgs {
              inherit system;
              config = { };
              overlays = [ ];
            }
          )
        );
      pname = "yamdw";
    in
    {
      devShells = eachSystem (pkgs: {
        default = pkgs.mkShellNoCC {
          packages = with pkgs; [ tinycc ];
        };
      });

      packages = eachSystem (
        pkgs:
        let
          fs = pkgs.lib.fileset;
          root = ./.;
          inherit (pkgs) coreutils dash tinycc;
          inherit (pkgs.lib) getExe getExe';
        in
        {
          default = derivation {
            name = "${pname}-0.0.1";
            inherit (pkgs) system;
            builder = getExe dash;
            args = [
              "-c"
              # TODO: static
              ''
                ${getExe' coreutils "mkdir"} -p "$out/bin" &&
                ${getExe tinycc} "$src/src/main.c" \
                  "$src/src/md4c/md4c.c" "$src/src/md4c/md4c.h" \
                  "$src/src/md4c/md4c-html.c" "$src/src/md4c/md4c-html.h" \
                  "$src/src/md4c/entity.c" "$src/src/md4c/entity.h" \
                  -o "${pname}" -std=c99 -fsanitize=address -O3 &&
                  # -Wall -Werror -Wextra
                ${getExe' coreutils "mv"} "${pname}" "$out/bin"
              ''
            ];
            src = fs.toSource {
              inherit root;
              fileset = fs.intersection (fs.gitTracked root) (
                fs.fileFilter (f: (f.hasExt "c") || (f.hasExt "h")) ./src
              );
            };
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
