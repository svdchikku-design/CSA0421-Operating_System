"""Q6: RAID capacity for 6 x 4 TB disks, probability of data loss, RAID 6 comparison, swap sizing."""
N, C = 6, 4.0
print("=== Usable capacity of 6 disks of 4 TB (24 TB raw) ===")
cap = {"RAID 0": N*C, "RAID 1 (3 mirrored pairs = RAID 10)": N/2*C, "RAID 1 (one 6-way mirror)": C, "RAID 5": (N-1)*C, "RAID 6": (N-2)*C, "RAID 10": N/2*C}
surv = {"RAID 0": "0", "RAID 1 (3 mirrored pairs = RAID 10)": "1 guaranteed, up to 3 (one per pair)", "RAID 1 (one 6-way mirror)": "5", "RAID 5": "1", "RAID 6": "2", "RAID 10": "1 guaranteed, up to 3 (one per pair)"}
for k, v in cap.items(): print(f"{k:38s} usable {v:5.1f} TB ({100*v/(N*C):5.1f}%)   disk failures survived: {surv[k]}")
print("\n=== Probability of data loss after one disk has failed ===")
afr, rebuild_h = 0.03, 24     # assumption: 3% annual failure rate, 24 h rebuild
p_h = 1 - (1 - afr) ** (rebuild_h / 8760)
print(f"assumptions: annual failure rate AFR = {afr:.0%} per disk, rebuild window = {rebuild_h} h -> P(a given disk fails during rebuild) = {p_h:.5f}")
r1 = p_h                                  # RAID 1 pair: only the partner disk matters
r5 = 1 - (1 - p_h) ** (N - 1)             # RAID 5: any of the other 5 disks
r6 = 1 - ((1 - p_h) ** (N - 1) + (N - 1) * p_h * (1 - p_h) ** (N - 2))  # RAID 6: needs 2 more failures
r10 = r1
print(f"RAID 1 / RAID 10 (partner must fail): {r1:.5f} = 1 in {1/r1:,.0f}")
print(f"RAID 5 (any of 5 others fails)        : {r5:.5f} = 1 in {1/r5:,.0f}  ({r5/r1:.1f}x worse than RAID 1)")
print(f"RAID 6 (two more must fail)           : {r6:.2e} = 1 in {1/r6:,.0f}")
ure = 1e-14
bits = (N - 1) * C * 1e12 * 8
pu = 1 - (1 - ure) ** bits
print(f"\nUnrecoverable read error while rebuilding RAID 5 (consumer drive 1 error per 1e14 bits): must read {5*C:.0f} TB = {bits:.2e} bits -> P(at least one URE) = {pu:.1%}")
bits1 = C * 1e12 * 8
print(f"RAID 1 rebuild reads only the {C:.0f} TB partner: P(URE) = {1-(1-ure)**bits1:.1%};  RAID 6 can still correct it with the second parity")
print(f"Enterprise drives (1 error per 1e15 bits): RAID 5 URE risk = {1-(1-1e-15)**bits:.1%}")
print("\n=== Write cost (disk operations per small write) ===")
print("RAID 0: 1   RAID 1/10: 2 (both copies)   RAID 5: 4 (read old data + old parity, write new data + new parity)   RAID 6: 6 (two parities)")
print("\n=== Swap sizing for the records server (assumption: 32 GB RAM, hibernation not used) ===")
ram = 32
print(f"rule of thumb: swap = 0.5 x RAM = {ram//2} GB for a server with large RAM (1 x RAM = {ram} GB only if hibernation is needed); peak-load headroom: add the size of the largest process that can burst above RAM")
