from flask import Flask, request, send_from_directory, render_template
from helper_funcs import *
import os

app = Flask(__name__)

#   Rendering the favicon of the website using send_from_directory
@app.route('/favicon.ico')
def favicon():
    return send_from_directory(os.path.join(app.root_path, 'static'),
                               'favicon.ico')

#   Implementation of basic routing using the functions above.
@app.route("/", methods=["GET", "POST"])
def index():
    #   If the HTTP request method is Post then try to get the lyrics 
    #   and render its template,
    #   otherwise return an error html page.
    if request.method == "POST":
        lyrics = get_lyrics(
            request.form["artist-input"], request.form["song-input"])
        if lyrics:
            return render_template(
                "lyrics.html",
                lyrics=lyrics,
                artist=request.form["artist-input"],
                title=request.form["song-input"],
            )
        else:
            return render_template("error.html")
    #   If the HTTP request method is not Post then get the hot tracks 
    #   and render index html page
    else:
        return render_template("index.html")

if __name__ == "__main__":
    app.run(debug=True)
