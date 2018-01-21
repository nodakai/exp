#!/usr/bin/env python3

import socket
import concurrent.futures
import subprocess
import functools
from typing import *


def kernel(param: Tuple[str, str], n_rep=5) -> Tuple[str, float]:
    host, addr = param
    r = subprocess.run(['ping', '-c', str(n_rep), addr],
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       timeout=6., check=True)
    latency_data_in_ms = [ float(s) for s in r.stdout.decode().split()[-2].split('/') ]
    _, mean_ms, _, stddev_ms = latency_data_in_ms
    percentile_95 = mean_ms + 1.96 * stddev_ms
    return (host, percentile_95)


def resolve(host: str) -> Iterable[str]:
    gai_res = socket.getaddrinfo(host, None, family=socket.AF_INET, type=socket.SOCK_STREAM)
    for _, _, _, __, (addr, _) in gai_res:
        yield addr


def main() -> None:
    pool = concurrent.futures.ThreadPoolExecutor()

    res_dict = {}  # type: Dict[str, List[float]]
    hosts = [
        'api.gdax.com',
        'api.kraken.com',
    ] # type: List[str]
    for host, latency in pool.map(kernel, ( (host, a) for host in hosts for a in resolve(host) )):
        buf = res_dict.get(host, [])  # List[float]
        buf.append(latency)
        res_dict[host] = buf
    latency_dict = {}  # type: Dict[str, float]
    for host, latency_arr in res_dict.items():
        geo_mean = functools.reduce(lambda x, y: x * y, latency_arr) ** (1/len(latency_arr))  # type: float
        latency_dict[host] = geo_mean
    for host, latency in latency_dict.items():
        print(f"{host}: {latency:.3}")


main()
