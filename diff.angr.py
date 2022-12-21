import sys

with open(sys.argv[1], 'rb') as f1, open(sys.argv[2], 'rb') as f2:
    # Read the contents of both files into variables
    contents1 = f1.read()
    contents2 = f2.read()

    # Compare the contents of the files
    if contents1 == contents2:
        print('The files are identical')
    else:
        # Find the first offset at which the files differ
        for i in range(min(len(contents1), len(contents2))):
            if contents1[i] != contents2[i]:
                print(f'The files differ at offset {i:x}')
                break
