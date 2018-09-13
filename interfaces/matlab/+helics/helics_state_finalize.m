function v = helics_state_finalize()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183055);
  end
  v = vInitialized;
end
