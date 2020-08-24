import re

# -ret16 = uint, int, bool...

# str = LPCSTR
# segstr = LPCSTR. don't know how it's different from str.
# ptr = LPVOID, or some other LP...
# segptr = void FAR*. Don't know how it's diifferent from ptr.
# word = uint
# s_word = int
# long = ulong

thelist = []

with open("gdi.spec", "r") as f:
    l = f.readlines()
    for x in l:
        ordnum, convention, r = x.split(None, 2)
        r = r.strip()

        if convention != 'pascal':
            continue

        if r[0] == '-':
            returntype, rr = r.split(None, 1)
        else:
            returntype = 'void'
            rr = r
        
        match = re.match(r'(.*?)\((.*?)\)', rr)
        fnname = match.group(1).strip()
        argsraw = match.group(2)
        args = argsraw.split(None)
        thelist.append({
            "ord": ordnum,
            "convention": convention,
            # return type seems unimportant
            "ret": returntype,
            "fn": fnname,
            "args": args
        })

for item in thelist:
    print(item)
