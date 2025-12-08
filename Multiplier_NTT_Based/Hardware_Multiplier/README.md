# Parametric NTT/INTT Hardware & Polynomial Multiplier Extension

This repository provides the baseline version of Verilog code for parametric NTT/INTT hardware published in "<a href="https://ieeexplore.ieee.org/document/9171507">An Extensive Study of Flexible Design Methods for the Number Theoretic Transform</a>", extended to include a full Polynomial Multiplier accelerator.

## Project Extension & Citation

This project is an extension of the design proposed by **A. C. Mert, E. Karabulut, E. Ozturk, E. Savas, and A. Aysu**.

If you use this repository in your research, please follow the citation guidelines below:

### 1. Using the Full Polynomial Multiplier
If you utilize the complete multiplier accelerator, please cite **both** works below (Ronald G. F. Silva & Marco A. Henriques **AND** Mert et al.):

> **RONALD G. F. SILVA & MARCO A. HENRIQUES**
> *"Implementation and Analysis of a Polynomial Product Accelerator on FPGA Based on the Number Theoretic Transform (NTT)"*

And:

> **A. C. Mert, E. Karabulut, E. Ozturk, E. Savas and A. Aysu**
> *"An Extensive Study of Flexible Design Methods for the Number Theoretic Transform"*

### 2. Using Only the NTT/INTT Modules
If you utilize only the baseline NTT/INTT modules, please cite only the original work:

```bibtex
@mastersthesis{Silva2025,
  author={Ronald G. F. {Silva} and Marco A. {Henriques}},
  title={Implementation and Analysis of a Polynomial Product Accelerator on FPGA Based on the Number Theoretic Transform (NTT)},
  school={University of Campinas (UNICAMP)},
  type={Bachelor's Thesis},
  address={Campinas, SP, Brazil},
  year={2025}
}

@ARTICLE{9171507,
  author={A. C. {Mert} and E. {Karabulut} and E. {Ozturk} and E. {Savas} and A. {Aysu}},
  journal={IEEE Transactions on Computers}, 
  title={An Extensive Study of Flexible Design Methods for the Number Theoretic Transform}, 
  year={2020},
  volume={},
  number={},
  pages={1-1},
  doi={10.1109/TC.2020.3017930}}
