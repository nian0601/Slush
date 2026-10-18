# Terminology

**Asset Repository**: a named folder under the top-level `Assets/` that holds authored content; a running game mounts an ordered list of them (game first, Engine always last), and file and asset lookups resolve through that list, first hit wins. Always written in full, never "repo".

**Debug script**: A text file or stream of debug-command lines run by the DebugScriptRunner.

**Debug command**: One named, registered operation a debug-script line invokes. Bare "Command" is reserved for the future undoable Command system.
