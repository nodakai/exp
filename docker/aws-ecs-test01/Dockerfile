FROM python:3-alpine

MAINTAINER NODA, Kai <nodakai@gmail.com>

WORKDIR /root

RUN apk --no-cache --update add tini iputils
# RUN pip3 install --no-cache-dir awscli
COPY app.py .

ENTRYPOINT ["/sbin/tini", "--"]
CMD ["./app.py"]
