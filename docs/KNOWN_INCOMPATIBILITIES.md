# Known Incompatibilities

> This document lists device, chipset, and BIOS combinations that are known to misbehave in LibreVMM. These are documented for reference—not restrictions. Users are free to use any combination but should be aware of potential issues.

**Note:** LibreVMM's philosophy is to warn, never block. If a combination is unusual or known problematic, a warning is displayed, but the configuration is allowed.

---

## How to Contribute

Community contributions are welcome! To add a new incompatibility:

1. **Identify the issue:** Document the exact combination that causes problems
2. **Describe the symptoms:** What happens when this combination is used?
3. **Explain the cause:** Why does this combination not work?
4. **Provide workarounds:** Are there alternative configurations that work?

Submit contributions via GitHub pull requests or issue reports.

---

## Format for Documenting Issues

Use the following format for each incompatibility entry:

```markdown
### Issue Title

**Affected Components:**
- Component A (version X.Y)
- Component B (version X.Y)

**Symptoms:**
- Description of the problem

**Cause:**
- Technical explanation

**Workaround:**
- Alternative configuration or fix
```

---

## Placeholder for Community Contributions

> **Your contribution here!**

The following areas need more testing and documentation:

- [ ] ARM64 guest combinations
- [ ] RISC-V guest combinations
- [ ] More legacy OS configurations (OS/2, BeOS, etc.)
- [ ] Firmware combinations with new chipsets
- [ ] virtio combinations with various guest OS versions

---

## Testing New Combinations

If you encounter an issue not listed here:

1. **Document the exact versions:** Include LibreVMM version, guest OS version, and device versions
2. **Identify the trigger:** What causes the issue? Is it consistent?
3. **Check for existing reports:** Search the issue tracker
4. **Try workarounds:** Document what you tried and what worked
5. **Submit:** Create an issue or pull request to add to this list

---

## Related Documentation

- [Priority 1 — Expanded Device Layer](../TODO.md#priority-1--expanded-device-layer)
- [Priority 4 — Remove VM Execution Restrictions](../TODO.md#priority-4---remove-vm-execution-restrictions)
- [DEVICE_PORTING_GUIDE.md](./DEVICE_PORTING_GUIDE.md)
- [EXECUTION_BACKENDS.md](./EXECUTION_BACKENDS.md)
