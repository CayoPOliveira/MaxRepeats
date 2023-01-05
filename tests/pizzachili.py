import os
import gzip
import urllib.request
import shutil


def download_files():
    urls = [
        "http://pizzachili.dcc.uchile.cl/texts/code/sources.100MB.gz",
        "http://pizzachili.dcc.uchile.cl/texts/protein/proteins.100MB.gz",
        "http://pizzachili.dcc.uchile.cl/texts/dna/dna.100MB.gz",
        "http://pizzachili.dcc.uchile.cl/texts/nlang/english.100MB.gz",
        "http://pizzachili.dcc.uchile.cl/texts/xml/dblp.xml.100MB.gz"
    ]

    if not os.path.exists("pizzagz"):
        os.makedirs("pizzagz")

    for link in urls:
        filename = os.path.join("pizzagz", link.split("/")[-1])
        if not os.path.exists(filename):
            print(f"Downloading {link} to {filename}")
            urllib.request.urlretrieve(link, filename)


def extract_and_split_file(filepath):
    with gzip.open(filepath, "rb") as f:
        content = f.read()
        content_str = content.decode("ISO-8859-1")
        content_str = content_str.replace("\n", " ")
        filename = os.path.splitext(os.path.basename(filepath))[
            0].split(".")[0]

        for i, size in enumerate([25, 50, 75, 100]):
            with open(f"pizza/{filename}.{size}MB.txt", "w") as f:
                f.write(content_str[:size*1024*1024])


def pizza():
    print("Pizzachili.py running...")
    if not os.path.exists("tests/pizza"):
        download_files()
        for filename in os.listdir("pizzagz"):
            extract_and_split_file(os.path.join("pizzagz", filename))
    if os.path.exists("pizzagz"):
        print("Deleting pizzagz")
        shutil.rmtree("pizzagz")


if __name__ == "__main__":
    pizza()
