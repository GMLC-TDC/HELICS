function v = next_step()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 12);
  end
  v = vInitialized;
end
