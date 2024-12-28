FROM python:3.12

WORKDIR /code

COPY "./projectptc/requirements.txt" /code/requirements.txt

RUN pip install --no-cache-dir -r /code/requirements.txt

COPY ./projectptc /code/projectptc

EXPOSE 8000

CMD [ "uvicorn", "projectptc.server:app", "--host", "0.0.0.0", "--port", "8000" ]