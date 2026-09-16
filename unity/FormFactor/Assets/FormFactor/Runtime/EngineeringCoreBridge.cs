using System;

namespace FormFactor.UnityFrontEnd
{
    public enum EngineeringValidationState
    {
        Unknown = 0,
        Pass = 1,
        Fail = 2
    }

    public readonly struct EngineeringValidationResult
    {
        public EngineeringValidationResult(EngineeringValidationState state, string message)
        {
            State = state;
            Message = message ?? string.Empty;
        }

        public EngineeringValidationState State { get; }
        public string Message { get; }
    }

    public interface IEngineeringCoreBridge
    {
        EngineeringValidationResult Validate(int componentCount, int connectionCount);
    }

    /// <summary>
    /// Fail-closed bridge used until the existing authoritative C++ engineering core
    /// is exported as a native library and connected to the Unity front end.
    /// The Unity renderer must never invent an engineering pass.
    /// </summary>
    public sealed class UnavailableEngineeringCoreBridge : IEngineeringCoreBridge
    {
        public EngineeringValidationResult Validate(int componentCount, int connectionCount)
        {
            return new EngineeringValidationResult(
                EngineeringValidationState.Unknown,
                $"ENGINEERING CORE NOT CONNECTED — {componentCount} PARTS / {connectionCount} VISUAL CONNECTIONS. VALIDATION REMAINS UNKNOWN.");
        }
    }
}
