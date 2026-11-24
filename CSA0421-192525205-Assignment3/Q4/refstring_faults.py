"""Page faults against number of frames for the textbook reference string (FIFO, LRU, Optimal)."""
import json
ref = [7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1]
def fifo(r,fr):
    f=[];fl=0
    for x in r:
        if x in f: continue
        fl+=1
        if len(f)<fr: f.append(x)
        else: f.pop(0); f.append(x)
    return fl
def lru(r,fr):
    f=[];fl=0
    for x in r:
        if x in f: f.remove(x); f.append(x); continue
        fl+=1
        if len(f)>=fr: f.pop(0)
        f.append(x)
    return fl
def opt(r,fr):
    f=[];fl=0
    for i,x in enumerate(r):
        if x in f: continue
        fl+=1
        if len(f)<fr: f.append(x); continue
        nxt=[(r[i+1:].index(y) if y in r[i+1:] else 99) for y in f]
        f[nxt.index(max(nxt))]=x
    return fl
print("frames FIFO LRU OPT  (20 references, 6 distinct pages)")
rows=[]
for fr in range(1,9):
    row=(fr,fifo(ref,fr),lru(ref,fr),opt(ref,fr)); rows.append(row); print(f"{fr:6d} {row[1]:4d} {row[2]:3d} {row[3]:3d}")
json.dump(rows,open("refstring_faults.json","w"))
