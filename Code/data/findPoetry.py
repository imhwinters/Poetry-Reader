import os
import requests
import re

NUM_POEMS = 1000

response = requests.get(
    f"https://poetrydb.org/random/{NUM_POEMS}",
    verify = False
)

database = response.json()

with open("index.csv", "w", encoding="utf-8") as index:
    index.write("title~author~file\n")

    for i, poem in enumerate(database):
        title = poem["title"]
        author = poem["author"]

        # Sanitize files
        filename = f"poem_{i}.txt"

        with open(filename, "w", encoding="utf-8") as f:
            f.write(f"{title}\n")
            f.write(f"by {author}\n\n")

            for line in poem["lines"]:
                f.write(line + "\n")

        index.write(
            f"{title.replace('~', ',')}~"
            f"{author.replace('~', ',')}~"
            f"{filename.replace('~', ',')}\n"
        )

        print(f"Downloaded: {title} by {author}")

print("Poetry installation complete.")