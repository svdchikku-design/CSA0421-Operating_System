# Slimming idea: build wheels in a full image, copy only the installed packages into the slim runtime image
FROM python:3.12 AS build
COPY requirements.txt .
RUN pip install --no-cache-dir --prefix=/install -r requirements.txt
FROM python:3.12-slim
COPY --from=build /install /usr/local
WORKDIR /app
COPY app.py .
RUN useradd --system --uid 10001 portal
USER portal
EXPOSE 8000
CMD ["gunicorn", "--bind", "0.0.0.0:8000", "app:app"]
