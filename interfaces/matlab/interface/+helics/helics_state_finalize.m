function v = helics_state_finalize()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 128);
  end
  v = vInitialized;
end
