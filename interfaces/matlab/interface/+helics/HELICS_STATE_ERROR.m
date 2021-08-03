function v = HELICS_STATE_ERROR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 134);
  end
  v = vInitialized;
end
