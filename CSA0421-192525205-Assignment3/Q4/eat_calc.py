"""Q4: effective access time with page faults. memory access 100 ns, fault service 8 ms."""
m, f = 100.0, 8_000_000.0
print("=== EAT = (1-p) x 100 ns + p x 8,000,000 ns ===")
for label, p in (("1 in 1,000", 1e-3), ("1 in 100,000", 1e-5), ("1 in 1,000,000", 1e-6)):
    eat = (1 - p) * m + p * f
    print(f"p = {label:>14}: EAT = {eat:12,.1f} ns  -> {eat/m:8.1f}x slower than 100 ns ({100*(eat/m-1):10,.0f}% slow-down)")
p10 = (0.10 * m) / (f - m)
print(f"\nMax page-fault rate for a 10% slow-down: (1-p)*100 + p*8e6 <= 110  ->  p <= 10/(8e6-100) = {p10:.3e} = 1 fault in {1/p10:,.0f} accesses")
print("\n=== Alternative: 32 GB machine (no faults after loading) vs demand paging at the measured rate ===")
for p in (1e-4, 1e-5, 1e-6):
    eat = (1 - p) * m + p * f
    print(f"measured-like rate p={p:.0e}: EAT {eat:8.1f} ns vs {m:.0f} ns with enough RAM -> run time x{eat/m:.2f}")
