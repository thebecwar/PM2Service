MessageIdTypedef=DWORD

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Error=0x3:STATUS_SEVERITY_ERROR
    )


FacilityNames=(System=0x0:FACILITY_SYSTEM
    Runtime=0x2:FACILITY_RUNTIME
    Stubs=0x3:FACILITY_STUBS
    Io=0x4:FACILITY_IO_ERROR_CODE
)

; // The following are message definitions.

MessageId=0x1
Severity=Error
Facility=Runtime
SymbolicName=SVC_ERROR
Language=English
An error has occurred (%1).
Message: %2
.

MessageId=0x2
Severity=Informational
Facility=Runtime
SymbolicName=SVC_INFO
Language=English
%1
.

MessageId=0x3
Severity=Warning
Facility=Runtime
SymbolicName=SVC_WARN
Language=English
%1
.

MessageId=0x4
Severity=Success
Facility=Runtime
SymbolicName=SVC_SUCCESS
Language=English
%1
.



; // A message file must end with a period on its own line
; // followed by a blank line.