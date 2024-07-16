# Gotchas

1. Failing to use IPv6: `sudo sysctl net.ipv6.conf.all.disable_ipv6=1`
2. Time sync issues: `sudo ntpdate -u time.nist.gov`

Restart: `sudo /usr/local/etc/rc.d/init_curtains_mq_client.sh restart`
