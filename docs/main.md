```mermaid
flowchart TD
    S1([Start])
    S2([Stop])
    PC[/Parse config file/]
    PA[/Parse command arguments/]
    CC[Create Connection]
    IS[Initialize Server]
    SS[Start Server]
    ST{Start?}
    GE[/Get events/]
    HE[Handle event]
    IT{Is terminating?}
    CU[Cleanup]
    S1 --> PC
    PC --> PA
    PA --> ST
    ST --> |true| CC
    ST --> |false| S2
    CC --> IS
    IS --> SS
    SS --> ML
    IT --> |true| CU
    CU --> S2
    subgraph ML [Main Loop]
        direction LR
        IT --> |false| GE
        GE --> HE
        HE --> IT
    end
```