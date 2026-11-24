"""B4: capacity estimate for result day, using the requests/second MEASURED in B3 and clearly labelled assumptions."""
import math
students, window_h = 150_000, 2
req_per_student = 5          # assumption: login page, result page, CSS/JS, PDF link, logout
peak_factor = 5              # assumption: busiest minute is 5x the 2-hour average (everyone refreshes when results are published)
measured_rps_1cpu, measured_rps_half = 1187, 591    # B3 measurement, trivial in-memory handler, 8 clients
work_factor = 10             # assumption: real handler (database query, template, TLS) is 10x more expensive than the stand-in
total = students * req_per_student; avg = total / (window_h * 3600); peak = avg * peak_factor
print(f"requests: {students:,} x {req_per_student} = {total:,}; average {avg:.0f} req/s; peak ({peak_factor}x) {peak:.0f} req/s")
per_pod = measured_rps_half / work_factor
print(f"one pod limited to 0.5 CPU: measured {measured_rps_half} req/s on the stand-in -> ~{per_pod:.0f} req/s for the real handler (/{work_factor})")
pods = math.ceil(peak / per_pod); print(f"pods for the peak: {pods}; with 30 % headroom: {math.ceil(pods*1.3)}  -> CPU requested {math.ceil(pods*1.3)*0.25:.1f} cores (requests), up to {math.ceil(pods*1.3)*0.5:.1f} cores (limits), memory {math.ceil(pods*1.3)*0.5:.1f} GiB (limit 512 MiB each)")
print(f"One physical server with 8 cores (assume 6 usable for the portal, 1,187 req/s per core / {work_factor}): {6*measured_rps_1cpu/work_factor:.0f} req/s -> {'enough' if 6*measured_rps_1cpu/work_factor>=peak else 'NOT enough'} for the peak, but no redundancy")
n = math.ceil(pods*1.3); cost = 0.03
print(f"Cost idea: the extra pods are needed only for about the 2 hours of result day; if one pod costs ${cost}/hour (assumption), {n} pods x 2 h = ${n*2*cost:.2f} per result day (plus load balancer, database and traffic costs)")
