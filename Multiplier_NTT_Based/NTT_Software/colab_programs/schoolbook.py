# NEGACYCLIC POLYNOMIAL MULTIPLICATION IN Z_q[x]/(x^n + 1)
# Colab-ready. Assumes coeff files are space-separated ints (possibly multiple lines).

from google.colab import files
import math

# --------- utils for IO ----------
def read_polynomial(filename):
    coeffs = []
    with open(filename, "r") as f:
        for line in f:
            parts = line.strip().split()
            if parts:
                coeffs.extend(int(x) for x in parts)
    return coeffs

def write_polynomial(filename, coeffs, per_line=10):
    with open(filename, "w") as f:
        for i in range(0, len(coeffs), per_line):
            f.write(" ".join(str(c) for c in coeffs[i:i+per_line]) + "\n")

# --------- negacyclic multiply ----------
def negacyclic_multiply(a, b, n, q):
    """
    Compute product in Z_q[x]/(x^n + 1).
    a, b: lists of length n (coeffs a[0] + a[1] x + ... + a[n-1] x^{n-1})
    returns list of length n with result coefficients mod q.
    """
    # Compute full convolution (length up to 2n-1)
    conv = [0] * (2*n - 1)
    for i in range(n):
        ai = a[i] % q
        if ai == 0:
            continue
        for j in range(n):
            conv[i + j] += ai * (b[j] % q)
    # reduce mod q (keep values smaller while summing)
    for k in range(len(conv)):
        conv[k] %= q

    # Negacyclic reduction: res[k] = (conv[k] - conv[k+n]) mod q
    res = [0] * n
    for k in range(n):
        high = conv[k + n] if (k + n) < len(conv) else 0
        res[k] = (conv[k] - high) % q
    return res

# --------- MAIN (Colab) ----------
# 1) Upload your two files (e.g., coeficientes_a.txt and coeficientes_b.txt)
print("Please upload the two coefficient files (space-separated ints).")
uploaded = files.upload()  # interactively upload files in Colab

# Identify filenames from uploaded
fnames = list(uploaded.keys())
if len(fnames) < 2:
    raise SystemExit("Upload exactly two files (or at least two).")

file1 = fnames[0]
file2 = fnames[1]
print("Using files:", file1, "and", file2)

# Parameters
q = 12289            # modulus (you used 12289 before)
n = 256              # target ring degree (change if needed)

# Read
a = read_polynomial(file1)
b = read_polynomial(file2)
print("Read lengths (before pad/trunc):", len(a), len(b))

# Ensure both have length n: pad with zeros or truncate
def fit_to_n(poly, n):
    if len(poly) < n:
        return poly + [0] * (n - len(poly))
    if len(poly) > n:
        return poly[:n]
    return poly

a = fit_to_n(a, n)
b = fit_to_n(b, n)
print("Using n =", n, "; lengths after fit:", len(a), len(b))

# Compute negacyclic product
result = negacyclic_multiply(a, b, n, q)
print("Computed negacyclic product modulo", q, "— result length:", len(result))

# Save and offer download
outname = "resultado_ring.txt"
write_polynomial(outname, result, per_line=10)
print("Saved:", outname)
files.download(outname)
