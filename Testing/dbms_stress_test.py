import subprocess

os = []
for i in range(100):
    c = f"curl -i -X POST http://localhost:9000/LOG_WRITE/{i+1}/1/TRANSACTION -d COMMIT"

    os.append(subprocess.Popen(c.split(' ')))

for o in os:
    o.wait()