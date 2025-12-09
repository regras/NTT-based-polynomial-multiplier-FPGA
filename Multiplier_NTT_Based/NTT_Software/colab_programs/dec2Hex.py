
def convert_to_hex(input_file, output_file):
    hex_values = []

    with open(input_file, "r") as f:
        for line in f:
            parts = line.strip().split()
            for p in parts:
                try:
                    value = int(p)
                    hex_values.append(format(value, "x"))  
                except ValueError:
                    print(f"Valor inválido ignorado: {p}")

    with open(output_file, "w") as f:
        for hv in hex_values:
            f.write(hv + "\n")

    print(f"\nConversão completa! {len(hex_values)} valores convertidos.")
    print(f"Arquivo gerado: {output_file}")


# --------------------------------------------
# Upload of file
# --------------------------------------------
from google.colab import files
print("Faça upload do arquivo de entrada (.txt):")
uploaded = files.upload()


input_filename = list(uploaded.keys())[0]
output_filename = "saida_hex_a.txt"

# --------------------------------------------
# Execute conversion
# --------------------------------------------
convert_to_hex(input_filename, output_filename)

# --------------------------------------------
# download of converted file
# --------------------------------------------
print("\nClique abaixo para baixar o arquivo convertido:")
files.download(output_filename)
