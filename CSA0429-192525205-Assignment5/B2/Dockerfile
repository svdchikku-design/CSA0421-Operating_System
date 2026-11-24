# B2 - Complete the Code: containerise the result portal
FROM python:3.12-slim
# TODO 1 (completed): working directory inside the image
WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
# TODO 2 (completed): copy the source and run as a non-root user called portal
COPY app.py .
RUN useradd --system --uid 10001 portal && chown -R portal /app
USER portal
# TODO 3 (completed): document the port and start the production server
EXPOSE 8000
CMD ["gunicorn", "--workers", "2", "--bind", "0.0.0.0:8000", "app:app"]
