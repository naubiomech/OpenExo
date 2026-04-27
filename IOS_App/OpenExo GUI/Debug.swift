import Foundation

/// Centralised debug-print helper.
///
/// Every diagnostic ``print()`` in the app routes through ``dprint``, so the
/// chatty stdout traces we used during development can be silenced (default)
/// or re-enabled in one place by flipping ``DEBUG_PRINTS`` to ``true``.
///
/// Errors that previously logged via ``print`` are routed the same way — flip
/// the flag if you need to see them in the Xcode console.  Data-rate stats
/// have been removed entirely.
enum DebugConfig {
    /// Set to ``true`` to print every ``dprint`` call.
    static var DEBUG_PRINTS: Bool = false
}

/// `print`-compatible wrapper that no-ops when ``DebugConfig.DEBUG_PRINTS`` is false.
@inlinable
func dprint(
    _ items: Any...,
    separator: String = " ",
    terminator: String = "\n"
) {
    guard DebugConfig.DEBUG_PRINTS else { return }
    let line = items.map { "\($0)" }.joined(separator: separator)
    Swift.print(line, terminator: terminator)
}
