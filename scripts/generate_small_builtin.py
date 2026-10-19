"""Generate reviewed Oxanium HUD glyphs without original game pixels."""
import argparse,json
from generate_label_pixels import generate_family,write_family
def generate():return generate_family('small')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--check',action='store_true');a=p.parse_args()
 print(json.dumps(write_family('small',a.check)))
if __name__=='__main__':main()
