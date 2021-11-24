function v = HELICS_STATE_PENDING_FINALIZE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 147);
  end
  v = vInitialized;
end
