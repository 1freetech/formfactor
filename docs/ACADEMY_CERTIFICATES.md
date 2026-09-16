# FormFactor Academy and Certificates

FormFactor includes a 100-lesson engineering academy that starts with basic circuit skills and moves toward more complex PCB, measurement, systems, and open-source engineering work.

## Difficulty rule

Lesson 1 begins at a 1.00x difficulty multiplier. Every following lesson is exactly 1.25% harder than the lesson before it using a compounding curve:

`difficulty(lesson) = 1.0125^(lesson - 1)`

That makes lesson 100 approximately 3.42x the starting difficulty. The playable lesson gate also increases practical requirements over time, including connected-path length, number of placed parts, number of different component types, and phase-specific component requirements.

## Certificate tiers

Every milestone earns a digital completion certificate:

| Completed lessons | Tier | Certificate |
| ---: | --- | --- |
| 10 | Tier I | FormFactor Foundations |
| 25 | Tier II | Circuit Builder |
| 50 | Tier III | PCB Systems |
| 75 | Tier IV | Advanced Open Hardware |
| 100 | Tier V | Open Source Engineer |

The 100-lesson **Open Source Engineer** certificate is the top FormFactor academy credential.

Certificates include the learner's name, milestone, issue date, and a generated FormFactor certificate ID. A local HTML copy is written under the game's user-data `certificates` directory. These are project-issued completion credentials and are not professional engineering licensure.

## Email delivery

The game can POST earned certificates to a configured certificate-mail service. Set either the Godot project setting `formfactor/certificate_endpoint` or the `FORMFACTOR_CERTIFICATE_ENDPOINT` environment variable. The environment variable takes priority.

The endpoint receives JSON containing the issuer, credential type, certificate title and tier, completed lesson count, recipient name and email, certificate ID, issue date, and the complete certificate HTML. A 2xx response marks that certificate as emailed. Failed or unconfigured delivery does not destroy the certificate; the local copy remains available and the user can retry with **CLAIM EARNED CERTS**.

No build should claim that a certificate was emailed unless the configured delivery endpoint returns a successful 2xx response.
