"""University result portal (small stand-in): GET /result/<reg_no>, GET /health."""
from flask import Flask, jsonify
import logging
logging.getLogger('werkzeug').setLevel(logging.ERROR)   # no per-request log lines
app = Flask(__name__)
RESULTS = {str(192525000 + i): {"reg_no": str(192525000 + i), "gpa": round(6 + (i % 40) / 10, 1), "status": "PASS" if i % 13 else "RE-APPEAR"} for i in range(5000)}

@app.route("/health")
def health():
    return "ok"

@app.route("/result/<reg>")
def result(reg):
    r = RESULTS.get(reg)
    return (jsonify(r), 200) if r else (jsonify(error="not found"), 404)

if __name__ == "__main__":
    import sys
    app.run(host="127.0.0.1", port=int(sys.argv[1]) if len(sys.argv) > 1 else 8000, threaded=True)
