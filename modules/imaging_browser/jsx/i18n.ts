import i18n from "i18next";
import { initReactI18next } from "react-i18next";

// the translations
// (tip move them in a JSON file and import them,
// or even better, manage them separated from your code: https://react.i18next.com/guides/multiple-translation-files)
const resources = {
  fr: {
    translation: {
      "Change language": "Changer de langue",
      "No data available": "Pas de données disponibles",
      " file(s) displayed": " fichier(s) affiché(s)",
      "QC Status": "Statut CQ",
      "Patient Name": "Nom du patient",
      "PSCID": "PSCID",
      "DCCID": "DCCID",
      "Visit Label": "Étiquette de visite",
      "Site": "Site",
      "QC Pending": "CQ en attente",
      "DOB": "DDN",
      "Sex": "Sexe",
      "Output Type": "Type de sortie",
      "Scanner": "Scanner",
      "Cohort": "Cohorte",
      "EDC": "DPC",
      "Selected": "Séléctionné",
      "Caveat": "Mise en garde",
      "True": "Vrai",
      "False": "Faux",
      "Longitudinal View": "Vue longitudinale",
      "Show Headers": "Montrer les en-têtes",
      "Hide Headers": "Cacher les en-têtes",
      "QC Comments": "Commentaires QC",
      'Download Image': "Télécharger l'image",
      "Download XML Protocol": "Télécharger le protocole XML",
      "Download XML Report": "Télécharger le rapport XML",
      "Download NRRD": "Télécharger le NRRD",
      "Download NIfTI": "Télécharger le NIfTI",
      "Download BVAL": "Télécharger le BVAL",
      "Download BVEC": "Télécharger le BVEC",
      "Download BIDS JSON": "Télécharger le BIDS JSON",
    }
  }
};

i18n
  .use(initReactI18next) // passes i18n down to react-i18next
  .init({
    resources,
    lng: localStorage.getItem('language') || 'en',
    interpolation: {
      escapeValue: false // react already safes from xss
    }
  });

(window as any).i18n = i18n;

export default i18n;
