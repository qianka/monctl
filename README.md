## minimal requirementst

* modern GCC

```
$ make
$ make install
```


## example config


* `/etc/monctl/monctl.conf`

```
dir = /var/lib/monctl
log_dir = /var/log/monctl
sleep = 2
attempts = 0
apps = /etc/monctl/apps.d/*.conf
```

* `/etc/monctl/apps.d/example.conf`

```
name = example
user = nobody
command = while true; do sleep 1; done
stdout = /tmp/exmaple.out
stderr = /tmp/exmaple.err
sleep = 2
attempts = 30
```
