function v = helics_state_finalize()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 23);
  end
  v = vInitialized;
end
