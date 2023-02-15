Japanese version is [README.ja.md](README.ja.md).

# windows_vds_exporter

**windows_vds_exporter** is a Prometheus exporter for Windows storage by using [Virtual Disk Service (VDS)](https://learn.microsoft.com/en-us/windows/win32/vds/virtual-disk-service-portal). Its purpose is to monitor software RAID health.

## How to use

windows_vds_exporter itself does not run as a web server. Instead, use it as:

- A CGI program. Use with a web server which has CGI host feature.
- A textfile exporter. Use with [windows_exporter](https://github.com/prometheus-community/windows_exporter)'s [textfile collector](https://github.com/prometheus-community/windows_exporter/blob/master/docs/collector.textfile.md).

In both cases, it requires SYSTEM privilege.

### As a CGI program

If `REQUEST_METHOD` environment variable is set, windows_vds_exporter runs as a CGI program.

Configure your web server so that windows_vds_exporter runs as a CGI program.

### As a textfile exporter

If `REQUEST_METHOD` environment variable is not set, windows_vds_exporter runs as a textfile exporter.

Run windows_vds_exporter periodically by task scheduler and write its stdout to the textfile directory of windows_exporter.

## Metrics

The following metrics are exposed.

| Name                                | Description                                                  | Labels                  | Value                                         |
| ----------------------------------- | ------------------------------------------------------------ | ----------------------- | --------------------------------------------- |
| windows_vds_volume_info             | Basic volume information                                     | name, type              | 1                                             |
| windows_vds_volume_size_bytes       | Volume size in bytes                                         | name                    | Volume size in bytes                          |
| windows_vds_volume_status           | Volume status                                                | name, status            | 1                                             |
| windows_vds_volume_transition_state | Volume transition state                                      | name, transition_state  | 1                                             |
| windows_vds_volume_health           | Volume health                                                | name, health            | 1                                             |
| windows_vds_volume_health_healthy   | Volume health                                                | name                    | 1 if `health` is `Healthy`.<br />0 otherwise. |
| windows_vds_volume_access_path      | Volume access paths (including paths through reparse points) | name, access_path       | 1                                             |
| windows_vds_volume_reparse_point    | Volume reparse points                                        | name, source_name, path | 1                                             |

## Output example

Suppose the following situation and "A Span Volume" is also mounted on E:\mnt:

![Disk Management View](diskman.png)

The output will be:

```
# HELP windows_vds_volume_info Volume information
# TYPE windows_vds_volume_info gauge
windows_vds_volume_info{name="\\\\?\\GLOBALROOT\\Device\\CdRom0",type="Simple"} 1.000000
windows_vds_volume_info{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10",type="Simple"} 1.000000
windows_vds_volume_info{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume11",type="Stripe"} 1.000000
windows_vds_volume_info{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume12",type="Mirror"} 1.000000
windows_vds_volume_info{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume3",type="Simple"} 1.000000
windows_vds_volume_info{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9",type="Span"} 1.000000

# HELP windows_vds_volume_size_bytes Volume size in bytes
# TYPE windows_vds_volume_size_bytes gauge
windows_vds_volume_size_bytes{name="\\\\?\\GLOBALROOT\\Device\\CdRom0"} 0.000000
windows_vds_volume_size_bytes{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10"} 4194304000.000000
windows_vds_volume_size_bytes{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume11"} 11639193600.000000
windows_vds_volume_size_bytes{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume12"} 3670016000.000000
windows_vds_volume_size_bytes{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume3"} 68038467072.000000
windows_vds_volume_size_bytes{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9"} 8178892800.000000

# HELP windows_vds_volume_status Volume status
# TYPE windows_vds_volume_status gauge
windows_vds_volume_status{name="\\\\?\\GLOBALROOT\\Device\\CdRom0",status="NoMedia"} 1.000000
windows_vds_volume_status{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10",status="Online"} 1.000000
windows_vds_volume_status{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume11",status="Online"} 1.000000
windows_vds_volume_status{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume12",status="Online"} 1.000000
windows_vds_volume_status{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume3",status="Online"} 1.000000
windows_vds_volume_status{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9",status="Online"} 1.000000

# HELP windows_vds_volume_transition_state Volume transition state
# TYPE windows_vds_volume_transition_state gauge
windows_vds_volume_transition_state{name="\\\\?\\GLOBALROOT\\Device\\CdRom0",transition_state="Stable"} 1.000000
windows_vds_volume_transition_state{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10",transition_state="Stable"} 1.000000
windows_vds_volume_transition_state{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume11",transition_state="Stable"} 1.000000
windows_vds_volume_transition_state{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume12",transition_state="Stable"} 1.000000
windows_vds_volume_transition_state{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume3",transition_state="Stable"} 1.000000
windows_vds_volume_transition_state{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9",transition_state="Stable"} 1.000000

# HELP windows_vds_volume_health Volume health
# TYPE windows_vds_volume_health gauge
windows_vds_volume_health{health="FailedRedundancy",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume12"} 1.000000
windows_vds_volume_health{health="Healthy",name="\\\\?\\GLOBALROOT\\Device\\CdRom0"} 1.000000
windows_vds_volume_health{health="Healthy",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10"} 1.000000
windows_vds_volume_health{health="Healthy",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume11"} 1.000000
windows_vds_volume_health{health="Healthy",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume3"} 1.000000
windows_vds_volume_health{health="Healthy",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9"} 1.000000

# HELP windows_vds_volume_health_healthy Volume health
# TYPE windows_vds_volume_health_healthy gauge
windows_vds_volume_health_healthy{name="\\\\?\\GLOBALROOT\\Device\\CdRom0"} 1.000000
windows_vds_volume_health_healthy{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10"} 1.000000
windows_vds_volume_health_healthy{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume11"} 1.000000
windows_vds_volume_health_healthy{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume12"} 0.000000
windows_vds_volume_health_healthy{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume3"} 1.000000
windows_vds_volume_health_healthy{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9"} 1.000000

# HELP windows_vds_volume_access_path Volume access path
# TYPE windows_vds_volume_access_path gauge
windows_vds_volume_access_path{access_path="C:\\",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume3"} 1.000000
windows_vds_volume_access_path{access_path="D:\\",name="\\\\?\\GLOBALROOT\\Device\\CdRom0"} 1.000000
windows_vds_volume_access_path{access_path="E:\\",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10"} 1.000000
windows_vds_volume_access_path{access_path="E:\\mnt\\",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9"} 1.000000
windows_vds_volume_access_path{access_path="F:\\",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume12"} 1.000000
windows_vds_volume_access_path{access_path="G:\\",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume11"} 1.000000
windows_vds_volume_access_path{access_path="H:\\",name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9"} 1.000000

# HELP windows_vds_volume_reparse_point Volume reparse point
# TYPE windows_vds_volume_reparse_point gauge
windows_vds_volume_reparse_point{name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume9",path="\\mnt",source_name="\\\\?\\GLOBALROOT\\Device\\HarddiskVolume10"} 1.000000
```

## Limitations

- Only software providers are supported. Other types of providers, i.e. hardware providers and virtual disk providers, are not supported and their metrics are not exposed at all.

## Known problems

- windows_vds_exporter does not seem to run under IIS. Please try other web servers.

## Copyright Notice and License

Copyright &copy; 2023 UMEZAWA Takeshi

**windows_vds_exporter** is licensed under GNU General Public License (GNU GPL) version 2 or later. See [LICENSE](LICENSE) for more information.
