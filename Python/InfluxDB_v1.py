import influxdb
import influxdb_client, os, time
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

class InfluxDB:
    def __init__(self, bucket="AIoT"):
        self.token = "4S-CyOmTPUhSgUiUSf0A8Fn31keTgErGXCoxoN1makt745rOxeASQAczKo_9K8IK7i-BZDq6L2bg7Y7Qr20qLQ=="
        self.org = "Christ"
        self.url = "http://localhost:8086"
        # url = "http://10.21.70.16:8086"
        self.bucket = bucket
        self.client = influxdb_client.InfluxDBClient(url=self.url, token=self.token, org=self.org)
        self.write_api = None
        self.delete_api = None
        self.query_api = None

    def delete(self, measure, start, stop):
        if self.delete_api is None:
            self.delete_api = self.client.delete_api()

        #start = "1970-01-01T00:00:00Z"
        #stop = "2100-01-01T00:00:00Z"
        return self.delete_api.delete(start, stop, "_measurement="+measure, bucket=self.bucket, org=self.org)

    def write(self, measure, atag, afield):
        if self.write_api is None:
            self.write_api = self.client.write_api(write_options=SYNCHRONOUS)

        point = (Point(measure).tag(atag[0], atag[1]).field(afield[0], afield[1]))
        return self.write_api.write(bucket=self.bucket, org=self.org, record=point)

    def query(self, measure, start):
        if self.query_api is None:
            self.query_api = self.client.query_api()

        query = f"""from(bucket: "{self.bucket}")
         |> range(start: {start})
         |> filter(fn: (r) => r._measurement == "{measure}")"""
        tables = self.query_api.query(query, org=self.org)

        return tables

    def query_mean(self, measure, start):
        if self.query_api is None:
            self.query_api = self.client.query_api()

        query = f"""from(bucket: "{self.bucket}")
         |> range(start: {start})
         |> filter(fn: (r) => r._measurement == "{measure}")
         |> mean()"""
        tables = self.query_api.query(query, org=self.org)

        return tables

if __name__ == '__main__':

    ts = InfluxDB(bucket="AIoT")
    measure = "measurement1"

    #ret = ts.write(measure, ["room", "room1"], ["temp", 20])
    #print(ret)

    #tables = ts.query(measure, "-30m")
    tables = ts.query_mean(measure, "-180d")
    print(tables)

    for table in tables:
        for record in table.records:
            print(record)
