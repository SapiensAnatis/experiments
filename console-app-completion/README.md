# Console app completion

Proof-of-concept for auto-generating Bash completions from apps written with [ConsoleAppFramework](https://github.com/Cysharp/ConsoleAppFramework).

In its current form, it loads a `.dll` assembly for a console app and invokes the `GetCliSchema` via reflection to introspect the commands.

## Usage

1. If you haven't already, add the `ConsoleAppFramework.CliSchema` package to your project; this causes ConsoleAppFramework to generate the `GetCliSchema` method that this tool relies on.
2. Invoke the tool with a path to the assembly and pipe the output to a script

```
dotnet run src/ -- /path/to/your/project/bin/Debug/net10.0/YourProject.dll > completions.sh
```

3. Source the completions manually or place them with other bash completions so that they are automatically sourced.

## Notes

The generated completions will invoke `complete` like this if run against `YourProject.dll`:

```
complete -o default -F __start_YourProject YourProject
```

If your tool's binary has a different name, or you add it to your `PATH` under an alias like `your-project`, then you will need to manually edit the completions to reflect this.

## Limitations

- This tool has not been tested against large apps.
- No completion for argument or option values is provided. Some completion may be possible against enum values, but otherwise it's difficult to know what values an option expects.

## Example

For a console app that I wrote, this tool will generate the following Bash script:

```bash
function __start_ModTools() {
  local cur="${COMP_WORDS[COMP_CWORD]}"
  local cmd="${COMP_WORDS[*]:1:COMP_CWORD-1}"
  case "$cmd" in
    "manifest add-event-assets")
      COMPREPLY=($(compgen -W "--target --target-locale --event-id --parsed-manifests-dir --output-manifest-dir --output-bundle-dir --assets-dir --conversion --read-from-disk" -- "$cur"))
      ;;
    "manifest edit-master")
      COMPREPLY=($(compgen -W "--master-path --output-path --read-from-disk" -- "$cur"))
      ;;
    "manifest add-bundles")
      COMPREPLY=($(compgen -W "--bundle-directory --output-path --read-from-disk" -- "$cur"))
      ;;
    "manifest decrypt")
      COMPREPLY=($(compgen -W "--read-from-disk" -- "$cur"))
      ;;
    "import-multiple")
      COMPREPLY=($(compgen -W "--directory --output-path --read-from-disk" -- "$cur"))
      ;;
    "manifest verify")
      COMPREPLY=($(compgen -W "--read-from-disk" -- "$cur"))
      ;;
    "manifest export")
      COMPREPLY=($(compgen -W "--output-path --read-from-disk" -- "$cur"))
      ;;
    "manifest import")
      COMPREPLY=($(compgen -W "--manifest-path --output-path --read-from-disk" -- "$cur"))
      ;;
    "manifest merge")
      COMPREPLY=($(compgen -W "--target-path --source-path --output-manifest-dir --output-bundle-dir --asset-directories --conversion --omissions-path --read-from-disk" -- "$cur"))
      ;;
    "check-target")
      COMPREPLY=($(compgen -W "--read-from-disk" -- "$cur"))
      ;;
    "update-names")
      COMPREPLY=($(compgen -W "--output-path --read-from-disk" -- "$cur"))
      ;;
    "manifest")
      COMPREPLY=($(compgen -W "decrypt edit-master merge verify export import add-bundles add-event-assets" -- "$cur"))
      ;;
    "convert")
      COMPREPLY=($(compgen -W "--output-path --read-from-disk" -- "$cur"))
      ;;
    "import")
      COMPREPLY=($(compgen -W "--asset-name --dictionary-path --output-path --inplace --read-from-disk" -- "$cur"))
      ;;
    "banner")
      COMPREPLY=($(compgen -W "--master-path --output-path --read-from-disk" -- "$cur"))
      ;;
    "hash")
      COMPREPLY=($(compgen -W "--read-from-disk" -- "$cur"))
      ;;
    "")
      COMPREPLY=($(compgen -W "check-target convert hash import import-multiple banner update-names manifest" -- "$cur"))
      ;;
  esac
}

complete -o default -F __start_ModTools ModTools
```
