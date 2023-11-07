from urllib.request import urlopen
from bs4 import BeautifulSoup

#   This function gets the artist name and song title as input and
#   returns the corresponding lyrics as output using Beautiful soup 
#   otherwise it will print the error message and thus will return an empty string.
def get_lyrics(artist, song):
    try:
        artist = f'{artist.replace(" ", "").lower()}'
        song = f'{song.replace(" ", "").lower()}'
        url = f"https://www.azlyrics.com/lyrics/{artist}/{song}.html"
        page = urlopen(url)
        html = page.read().decode("utf-8")
        soup = BeautifulSoup(html, "html.parser")
        main = soup.find(class_="col-xs-12 col-lg-8 text-center")
        divs = main.find_all("div")
        results = [(len(div.text), div.text.strip()) for div in divs]
        lyrics = max(results, key=lambda x: x[0])[1]
        return lyrics
    except Exception as e:
        print(e)
        return ""
