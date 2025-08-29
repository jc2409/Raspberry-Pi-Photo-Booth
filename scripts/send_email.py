#!/usr/bin/env python3
import os, sys, argparse, smtplib, mimetypes
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.application import MIMEApplication
from dotenv import load_dotenv

load_dotenv()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--to", required=True)
    ap.add_argument("--name", required=True)
    ap.add_argument("--file", required=True)  # full path to attachment
    args = ap.parse_args()

    sender_email = os.getenv("SENDER_EMAIL")
    sender_password = os.getenv("SENDER_PASSWORD")
    if not sender_email or not sender_password:
        print("Missing SENDER_EMAIL or SENDER_PASSWORD in environment.", file=sys.stderr)
        return 2

    subject = f"Arm Photo Booth — {args.name}"
    body = "This is the photo from the Arm Demo Photo Booth"

    msg = MIMEMultipart()
    msg["From"] = sender_email
    msg["To"] = args.to
    msg["Subject"] = subject
    msg.attach(MIMEText(body, "plain", "utf-8"))

    path = args.file
    if not os.path.isfile(path):
        print(f"Attachment not found: {path}", file=sys.stderr)
        return 3

    ctype, _ = mimetypes.guess_type(path)
    with open(path, "rb") as f:
        part = MIMEApplication(f.read(), _subtype=(ctype.split("/")[1] if ctype else "octet-stream"))
    part.add_header("Content-Disposition", "attachment", filename=os.path.basename(path))
    msg.attach(part)

    # Gmail SMTP over SSL (port 465). For 2FA accounts, use an App Password.
    try:
        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
            server.login(sender_email, sender_password)
            server.sendmail(sender_email, [args.to], msg.as_string())
        print("sent")
        return 0
    except Exception as e:
        print(f"SMTP error: {e}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    sys.exit(main())
