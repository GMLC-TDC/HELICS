function v = helics_state_finished()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 135);
  end
  v = vInitialized;
end
