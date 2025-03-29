import requests
import csv
from bs4 import BeautifulSoup as html_parser
import os, sys
from calendar import monthrange

def extractTable(url, date:str, dir:str):
    headers = {
        'X-Requested-With': 'XMLHttpRequest',
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:125.0) Gecko/20100101 Firefox/125.0',
        'Accept': '*/*',
        'Accept-Language': 'en-US,en;q=0.5',
        'Accept-Encoding': 'gzip, deflate, br',
        'Referer': 'https://portal.vietcombank.com.vn/en-Us/Corporate/TG/Pages/exchange-rate.aspx?devicechannel=default',
    }
    sess = requests.Session()
    resp = sess.get(url, headers=headers)

    if(resp.status_code != requests.codes.ok):
        return resp.status_code

    soup = html_parser(resp.text, 'lxml')

    if(os.path.exists(f"{dir}") == False):
        os.makedirs(f"{dir}")

    with open(f"{dir}/vcb-er-{date.replace("/", "")}_table.html", 'w+', encoding='utf-8') as html_file:
        html_file.write(soup.prettify())

def downloadData(month: int, year: int):
    numdays = monthrange(year, month)[1]
    for day in range(1, numdays + 1):
        date = f"{day:02}/{month:02}/{year}"
        vcb_link = f"https://portal.vietcombank.com.vn/UserControls/TVPortal.TyGia/pListTyGia.aspx?txttungay={date}&BacrhID=1&isEn=True"
        retStat = extractTable(vcb_link, date, f"vcbtables/{year}{month:02}")
        if(retStat != None):
            print(retStat)
            exit()

def main(argv: list, argc: int):
    '''
    pass the month and year
    1st arg: month
    2nd arg: year
    sample: python htmltocsv.py 5 2024
    where 5 is month of May, and year is 2024
    '''
    month = int(argv[1])
    year = int(argv[2])

    downloadData(month, year)

    headers = ['Day', 'Buying Rates']
    rate = str()
    data = list()
    rates = list()
        
    with open(f'vcbtables/erdata{month:02}{year}.csv', 'a+', encoding='utf-8', newline='') as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=headers, quoting=csv.QUOTE_MINIMAL)
        writer.writeheader()

        numdays = monthrange(year, month)[1]
        for day in range(1, numdays + 1):
            date = f"{day:02}{month:02}{year}"
            with open(f"vcbtables/{year}{month:02}/vcb-er-{date}_table.html", 'r', encoding='utf-8') as html_file:
                soup = html_parser(html_file.read(), 'lxml')

            table = soup.select_one("table tbody")
            if(table is None):
                continue

            rows = table.select("tr")
            for row in rows[2:]:
                cells = row.find_all("td")
                if(cells[1].text.strip() == "USD"):
                    # for cell in cells:
                    rate = cells[2].text.strip()
                    # rates = [cell.text.strip() for cell in cells]
                    # data.append(dict(zip(headers, rates)))


            writer.writerow({'Day': day, 'Buying Rates': rate})
        # writer.writerows(data)



if __name__ == "__main__":
    main(sys.argv, len(sys.argv))