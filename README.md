# WinApiSniffer

Work in progress 🔥

## Enable event collection

```PowerShell
# Create new trace session
New-EtwTraceSession -Name WinApiSniffer -LogFileMode 0x8100 -FlushTimer 1 -LocalFilePath "C:\WinApiSniffer.etl"
# Add Sniffer Trace Provider
Add-EtwTraceProvider -SessionName WinApiSniffer -Guid ‘{dad3e83e-90de-43ad-94e5-96a2b68d84a5}’ -MatchAnyKeyword 0x0FFFFFFFFFFFFFFF -Level 0xFF -Property 0x40

# !!! now inject the DLL and perform the actions you wish to capture !!!

# Finally stop/remove the session
Remove-EtwTraceSession -Name WinApiSniffer
```
