//! FormFactor native bridge helpers.
//!
//! This crate is deliberately narrow. It prepares deterministic frontend
//! snapshots for the authoritative C++ engineering core. It never decides
//! whether an electrical design passes.

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct PartPlacement {
    pub id: u64,
    pub kind: String,
    pub x_milli: i32,
    pub y_milli: i32,
    pub z_milli: i32,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum EngineeringStatus {
    Unknown,
    Pass,
    Fail,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct EngineeringResult {
    pub status: EngineeringStatus,
    pub message: String,
}

/// Return the safe frontend result when no authoritative C++ result exists.
pub fn unknown_without_core() -> EngineeringResult {
    EngineeringResult {
        status: EngineeringStatus::Unknown,
        message: "Engineering core result is unavailable; status remains UNKNOWN.".to_string(),
    }
}

/// Produce a stable, line-oriented snapshot record for the native core.
///
/// Records are sorted by stable component ID. Text fields are length-prefixed
/// so a frontend string cannot create ambiguous separators.
pub fn canonical_snapshot(parts: &[PartPlacement]) -> String {
    let mut sorted = parts.to_vec();
    sorted.sort_by_key(|part| part.id);

    let mut output = String::from("formfactor-frontend-snapshot-v1\n");
    for part in sorted {
        output.push_str(&format!(
            "{}:{}:{}:{}:{}:{}:{}\n",
            part.id,
            part.kind.len(),
            part.kind,
            part.x_milli,
            part.y_milli,
            part.z_milli,
            0
        ));
    }
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn snapshot_is_deterministic_and_sorted() {
        let parts = vec![
            PartPlacement {
                id: 9,
                kind: "LED".to_string(),
                x_milli: 600,
                y_milli: 280,
                z_milli: 0,
            },
            PartPlacement {
                id: 2,
                kind: "POWER".to_string(),
                x_milli: -1200,
                y_milli: 280,
                z_milli: 0,
            },
        ];

        let record = canonical_snapshot(&parts);
        let power = record.find("2:5:POWER").expect("POWER record");
        let led = record.find("9:3:LED").expect("LED record");
        assert!(power < led);
        assert_eq!(record, canonical_snapshot(&parts));
    }

    #[test]
    fn missing_core_never_becomes_pass() {
        let result = unknown_without_core();
        assert_eq!(result.status, EngineeringStatus::Unknown);
        assert!(!result.message.is_empty());
    }
}
