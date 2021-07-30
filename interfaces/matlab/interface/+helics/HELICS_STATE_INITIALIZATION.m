function v = HELICS_STATE_INITIALIZATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 131);
  end
  v = vInitialized;
end
