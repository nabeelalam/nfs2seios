"""Validate the optional fingerprint without interpreting it as shell or asset data."""
import os
import re
import sys


def validate(value, required=False):
    if required and not value:
        raise ValueError("Classic UI requires the reviewed artwork fingerprint")
    if value and re.fullmatch(r"[0-9a-fA-F]{64}", value) is None:
        raise ValueError("Artwork fingerprint must be empty or exactly 64 hexadecimal characters")


if __name__ == "__main__":
    try:
        validate(os.environ.get("NFS_HD_ARTWORK_SHA256", ""),
                 required=os.environ.get("NFS_CLASSIC_UI_REQUESTED") == "true")
    except ValueError as error:
        sys.exit(str(error))
