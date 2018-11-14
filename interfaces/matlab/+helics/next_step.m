function v = next_step()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230868);
  end
  v = vInitialized;
end
