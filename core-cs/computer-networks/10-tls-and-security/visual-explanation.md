# Visual Explanation: Diffie-Hellman Key Exchange and PKI Hierarchy

---

## 1. The Paint-Mixing Analogy of Diffie-Hellman Key Exchange

How two parties agree on a secret key across an open public channel:

```
Alice                                Public Eye (Eve)                       Bob
====================================================================================
1. Agree on Common Public Color:
   [ Yellow ] ========================================================> [ Yellow ]

2. Pick Secret Private Color:
   [ Secret: Orange ]                                                   [ Secret: Teal ]

3. Mix Secret + Common:
   Orange + Yellow =                                                    Teal + Yellow =
   [ Orange-Yellow ]                                                    [ Teal-Yellow ]

4. Exchange Mixed Colors Publicly:
   [ Orange-Yellow ] ==============> (Eve sees mixed!) ===============> [ Orange-Yellow ]
   [ Teal-Yellow ]   <============== (Eve cannot separate colors!) <=== [ Teal-Yellow ]

5. Add Private Secret to Received Mix:
   Orange + [Teal-Yellow] =                                             Teal + [Orange-Yellow] =
   [ SHARED SECRET: BROWN ]                                             [ SHARED SECRET: BROWN ]
====================================================================================
Result: Both Alice and Bob hold the IDENTICAL secret color (Brown).
Eve (eavesdropper) observed Yellow, Orange-Yellow, and Teal-Yellow, but cannot
determine the shared secret without inverting the one-way mixing function!
```

---

## 2. Certificate Chain of Trust Validation

```
Browser / OS Trust Store
+-------------------------------------------------------------+
| Root CA Certificate (DigiCert / Let's Encrypt ISRG Root X1) |
|   - Self-signed                                             |
|   - Pre-installed in OS ROM / Keyring                       |
+-------------------------------------------------------------+
       |
       | Signs Intermediate with Root Private Key
       v
+-------------------------------------------------------------+
| Intermediate CA Certificate (R3 / E1)                       |
|   - Validated using Root CA's Public Key                    |
+-------------------------------------------------------------+
       |
       | Signs Leaf with Intermediate Private Key
       v
+-------------------------------------------------------------+
| End-Entity Leaf Certificate (*.google.com)                  |
|   - Validated using Intermediate CA's Public Key            |
|   - Verified: Domain matches URL ("www.google.com")         |
|   - Verified: Current timestamp is within [NotBefore, NotAfter]
|   - Verified: Not listed on OCSP Revocation list            |
+-------------------------------------------------------------+
       |
       v
  [ Secure Green Padlock / HTTPS Connection Established ]
```
